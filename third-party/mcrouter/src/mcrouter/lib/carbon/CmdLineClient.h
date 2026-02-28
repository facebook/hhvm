/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>

#include "mcrouter/lib/carbon/JsonClient.h"

namespace carbon {
namespace tools {

/**
 * Helper class to be used by command-line tools debug tools to send requests to
 * Carbon Servers.
 * It provides a single method that accepts command line arguments, parses it,
 * and send requests.
 */
class CmdLineClient {
 public:
  explicit CmdLineClient(
      std::ostream& targetOut = std::cout,
      std::ostream& targetErr = std::cerr);

  /**
   * Parses the command line arguments and send the requests.
   * For information on supported command-line arguments, check
   * parseSettings() implementation.
   *
   * If something is wrong with the provided arguments, exit() may be called.
   * If everything works as expected, this method will return normally.
   *
   * @tparam Client   Type (that inherits from JsonClient) to be used
   *                  to send the requests.
   *
   * @param argc      Number of command line arguments in argv.
   * @param argv      Command line arguments.
   */
  template <class Client>
  void sendRequests(int argc, const char** argv);

 private:
  std::ostream& targetOut_;
  std::ostream& targetErr_;

  struct Settings {
    JsonClient::Options clientOptions;
    std::string requestName;
    std::string data;
  };

  Settings parseSettings(int argc, const char** argv);

  void sendRequests(
      JsonClient& client,
      const std::string& requestName,
      const std::string& data);
};

} // namespace tools
} // namespace carbon

#include "CmdLineClient-inl.h"
