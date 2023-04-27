/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/tool/Commands.h>
#include <folly/Conv.h>
#include <folly/portability/GFlags.h>
#include <folly/ssl/Init.h>
#include <glog/logging.h>

#include <iostream>
#include <string>
#include <vector>

DECLARE_string(vmodule);

using namespace fizz::tool;

void showUsage() {
  std::cerr << "Supported commands:" << std::endl;
  for (const auto& command : utilityNames) {
    std::cerr << "  - " << command << ": " << utilityDescriptions.at(command)
              << std::endl;
  }
  std::cerr << std::endl;
}

void parseVerbosityFlags(const std::vector<std::string>& arguments) {
  // Handle v + vmodule raw, as we need to do it before
  // logging is initialized.
  for (size_t i = 2; i < arguments.size(); i++) {
    if (arguments[i] == "-v") {
      if (i + 1 == arguments.size()) {
        throw std::runtime_error("-v requires an argument.");
      }
      FLAGS_v = folly::to<int32_t>(arguments[i + 1]);
    }

    if (arguments[i] == "-vmodule") {
      if (i + 1 == arguments.size()) {
        throw std::runtime_error("-vmodule requires an argument.");
      }
      FLAGS_vmodule = arguments[i + 1];
    }
  }
}

int main(int argc, char** argv) {
  std::vector<std::string> arguments;
  for (int i = 0; i < argc; i++) {
    arguments.push_back(argv[i]);
  }

  try {
    parseVerbosityFlags(arguments);
  } catch (const std::exception& e) {
    std::cerr << "Error parsing verbosity flags: " << e.what() << std::endl;
    return 1;
  }

  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  folly::ssl::init();

  if (arguments.size() < 2) {
    showUsage();
    return 1;
  } else {
    if (fizzUtilities.count(arguments[1])) {
      return fizzUtilities.at(arguments[1])(arguments);
    } else {
      std::cerr << "Unknown command '" << arguments[1] << "'." << std::endl;
      showUsage();
      return 1;
    }
  }
  return 0;
}
