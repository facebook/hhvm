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
  // folly::Init parses gflags (consuming flags like --v, --vmodule, --logging)
  // and initializes whichever logging backend is linked (glog or folly xlog).
  // It mutates argc/argv to strip parsed flags; the remaining args carry the
  // subcommand and its arguments.
  folly::Init init(&argc, &argv);

  std::vector<std::string> arguments;
  for (int i = 0; i < argc; i++) {
    arguments.emplace_back(argv[i]);
  }

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
