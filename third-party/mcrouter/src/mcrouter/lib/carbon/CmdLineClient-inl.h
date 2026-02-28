/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>

namespace carbon {
namespace tools {

template <class Client>
void CmdLineClient::sendRequests(int argc, const char** argv) {
  auto settings = parseSettings(argc, argv);
  Client jsonClient(settings.clientOptions, [this](const std::string& msg) {
    targetErr_ << msg << std::endl;
  });
  sendRequests(jsonClient, settings.requestName, settings.data);
}

} // namespace tools
} // namespace carbon
