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
#include "hphp/util/process.h"

namespace {

const std::string kProgramDescription = "Creates an index file for GDB";

}

int main(int argc, char** argv) {
  namespace po = boost::program_options;

  std::string output_file;
  int num_threads;

  po::options_description desc{"Allowed options"};
  desc.add_options()
    ("help", "produce help message")
    ("threads,j",
      po::value<int>(&num_threads)->
        default_value(std::max(1, ::HPHP::Process::GetCPUCount())),
      "number of threads")
    ("output,o",
     po::value<std::string>(&output_file)->required(),
     "output file");

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

    if (filenames.size() != 1) {
      std::cerr << "ERROR: Requires a single input file"
                << std::endl;
      return 1;
    }

    try {
      const auto indexer = GDBIndexer::make(filenames[0], num_threads);
      if (!indexer) {
        std::cerr << "ERROR: Platform doesn't have a debug-info parser"
                  << std::endl;
        return 1;
      }
      (*indexer)(output_file);
    } catch (const Exception& exn) {
      std::cerr << "ERROR: " << exn.what() << std::endl;
      return 1;
    }
  } catch (const po::error& e) {
    std::cerr << e.what() << "\n\n"
              << kProgramDescription << "\n\n"
              << desc << std::endl;
    return 1;
  }

  return 0;
}
