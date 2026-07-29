/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/tool/Commands.h>
#include <folly/init/Init.h>

#include <iostream>
#include <string>
#include <vector>

using namespace fizz::tool;

void showUsage() {
  std::cerr << "Supported commands:" << std::endl;
  for (const auto& command : utilityNames) {
    std::cerr << "  - " << command << ": " << utilityDescriptions.at(command)
              << std::endl;
  }
  std::cerr << std::endl;
}

int main(int argc, char** argv) {
  // The command line is: fizz [gflags...] <subcommand> [subcommand args...].
  // A subcommand's own args use single-dash flags (e.g. -connect) that gflags
  // would reject as unrecognized, which historically forced callers to insert a
  // "--" before the subcommand. To avoid that, split the command line at the
  // subcommand: folly::Init/gflags only sees the leading flags, while the
  // subcommand and its args are captured verbatim (order preserved) beforehand.
  int splitIdx = 1;
  while (splitIdx < argc && !fizzUtilities.contains(argv[splitIdx])) {
    splitIdx++;
  }

  std::vector<std::string> arguments;
  arguments.emplace_back(argv[0]);
  for (int i = splitIdx; i < argc; i++) {
    arguments.emplace_back(argv[i]);
  }

  // folly::Init parses gflags (consuming flags like --v, --vmodule, --logging)
  // and initializes whichever logging backend is linked (glog or folly xlog).
  // Bounding it to the leading flags keeps it from touching the subcommand
  // args.
  int initArgc = splitIdx;
  folly::Init init(&initArgc, &argv);

  if (arguments.size() < 2) {
    showUsage();
    return 1;
  }
  if (fizzUtilities.contains(arguments[1])) {
    return fizzUtilities.at(arguments[1])(arguments);
  }
  std::cerr << "Unknown command '" << arguments[1] << "'." << std::endl;
  showUsage();
  return 1;
}
